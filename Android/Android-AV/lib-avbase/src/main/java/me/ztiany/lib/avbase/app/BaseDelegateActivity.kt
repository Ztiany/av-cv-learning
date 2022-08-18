package me.ztiany.lib.avbase.app

import android.os.Bundle
import androidx.viewbinding.ViewBinding
import com.android.base.delegate.simpl.DelegateActivity
import me.ztiany.lib.avbase.utils.inflateBindingWithParameterizedType

open class BaseDelegateActivity<VB : ViewBinding> : DelegateActivity() {

    private lateinit var _binding: VB

    protected val binding: VB
        get() = _binding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        beforeSetUpView()
        _binding = inflateBindingWithParameterizedType(layoutInflater)
        setContentView(_binding.root)
        setUpView()
    }

    protected open fun beforeSetUpView() {

    }

    protected open fun setUpView() {

    }

}